package com.games.starwars.view.textures.ships;

import com.games.starwars.model.ships.StarShip;
import com.games.starwars.view.textures.TexturePack;
import com.games.starwars.view.textures.TextureImpl;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;

import java.util.ArrayList;

public class StarDestroyerTexture extends TextureImpl implements ShipTexture {
    private StarShip ship;
    private final ArrayList<Rectangle> healthPoints = new ArrayList<>();
    private double blockWidth = 0;
    private int startHP = 0;
    private int previousHP = 0;
    private final ArrayList<Rectangle> util = new ArrayList<>();
    private final double HEALTH_BAR_WIDTH = 60;
    private boolean healthBarIsSet = false;

    public StarDestroyerTexture() {
        super(0, 0, 0, 0);
    }

    public StarDestroyerTexture(StarShip s) {
        super(s.getX(), s.getY(), s.getWidth(), s.getHeight());
        getTexture().setFill(TexturePack.getStarDestroyerTexture(s.getCurrentDirection()));
        ship = s;
    }

    @Override
    public void appear(Pane pane) {
        pane.getChildren().add(getTexture());
    }

    @Override
    public Rectangle getTexture() {
        return super.getTexture();
    }

    @Override
    public void updateView(Pane pane) {
        setFill();
        getTexture().setX(ship.getX());
        getTexture().setY(ship.getY());
        if (ship.getHP() == 0) {
            pane.getChildren().removeAll(healthPoints);
            healthPoints.clear();
            return;
        }
        if (ship.getHP() == startHP) {
            return;
        }
        if (!healthBarIsSet) {
            healthBarIsSet = true;
            initHealthBar();
            if (0 == healthPoints.size()) {
                return;
            }
            for (int i = 0; i < startHP; i++) {
                pane.getChildren().add(healthPoints.get(i));
            }
        }
        updateHealthBar();
    }

    @Override
    public void removeFrom(Pane pane) {
        pane.getChildren().remove(getTexture());
        pane.getChildren().removeAll(healthPoints);
    }

    @Override
    public void setShip(StarShip ship) {
        this.ship = ship;
        this.startHP = ship.getHP();
        this.previousHP = startHP;
        setX(ship.getX());
        setY(ship.getY());
        setWidth(ship.getWidth());
        setHeight(ship.getHeight());
        setFill();
    }

    protected void setFill() {
        getTexture().setFill(TexturePack.getStarDestroyerTexture(ship.getCurrentDirection()));
    }

    protected StarShip getShip() {
        return ship;
    }

    protected void updateHealthBar() {
        double xOffset = (ship.getWidth() - HEALTH_BAR_WIDTH) / 2;
        double x = ship.getX() + xOffset;
        double y = ship.getY() + ship.getHeight() + 3;
        for (Rectangle healthPoint : healthPoints) {
            healthPoint.setX(x);
            healthPoint.setY(y);
            x += blockWidth;
        }
        util.clear();
        for (int i = ship.getHP(); i < startHP; i++) {
            healthPoints.get(i).setFill(Color.BLACK);
        }
        previousHP = ship.getHP();
    }

    protected void initHealthBar() {
        double height = 3;
        blockWidth = HEALTH_BAR_WIDTH / startHP;
        double xOffset = (ship.getWidth() - HEALTH_BAR_WIDTH) / 2;
        double x = ship.getX() + xOffset;
        double y = ship.getY() + ship.getHeight() + 3;
        for (int i = 0; i < startHP; i++) {
            Rectangle rect = new Rectangle(x, y, blockWidth, height);
            x += blockWidth;
            rect.setFill(Color.DARKRED);
            healthPoints.add(rect);
        }
    }
}